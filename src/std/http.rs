use reqwest;

pub enum HTTP {
    Get,
    Post,
    GetSync,        // Is functionally identical to  Get, although only synchronous.
    PostSync,       // Is functionally identical to Post, although only synchronous.
}

impl HTTP {
    pub async fn get(url: &str) -> Result<String, reqwest::Error> {
       reqwest::get(url).await?.text().await
    }

    pub async fn post(url: &str) {

    }

    pub fn get_sync(url: &str) {

    }

    pub fn post_sync(url: &str) {
        
    }
}       
