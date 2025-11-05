use rand::Rng;

/*
 * The built-in Math library for AdaN.
 * Provides functions and formulas for various things relating to math and more.
 *
 */
/*

    Usage:
    
    include adan.std.math; // Imports the math native library.
    
    program -> main {
        ...
    }

*/

enum Random {
    Rand,
}

impl Random {
    pub fn rand_f64() -> f64 {
        rand::thread_rng().gen::<f64>()
    }

    pub fn random_range_isize() -> i32 {
        rand::thread_rng().gen_range(min..=max)
    }

    pub fn random_range_fsize() -> f64 {
        rand::thread_rng().gen_range(min..max)
    }
}


enum Rounding {
    Ceiling,
    Floor,
    Round,
}

impl Rounding {
    pub fn ceil(n: f64) -> isize {
        n.ceil() as isize
    }

    pub fn floor(n: f64) -> isize {
        n.floor() as isize
    }

    pub fn round(n: f64) -> isize {
        n.round() as isize
    }
}


enum Algebraic {
    Abs,
    Power,
    Square,
    Cube,
    Sqrt,
    Cbrt,
    Log,
    Exp,
}

trait Pow {
    fn pow(self, exp: Self) -> Self;
}

impl Pow for i32 {
    fn pow(self, exp: Self) -> Self {
        self.pow(exp as u32)
    }
}

impl Pow for f64 {
    fn pow(self, exp: Self) -> Self {
        self.powf(exp)
    }
}

impl Algebraic {
    pub fn abs<T: num_traits::Signed>(n: T) -> T {
        n.abs()
    }

    pub fn pow<T: Pow>(n: T, x: T) -> T {
        n.pow(x)
    }

    pub fn square<T: Pow + From<i32>>(n: T) -> T {
        Self::pow(n, T::from(2))
    }

    pub fn cube<T: Pow + From<i32>>(n: T) -> T {
        Self::pow(n, T::from(3))
    }

    pub fn sqrt(n: f64) -> f64 {
        n.sqrt()
    }

    pub fn cbrt(n: f64) -> f64 {
        n.cbrt()
    }

    pub fn log(n: f64, base: f64) -> f64 {
        n.log(base)
    }

    pub fn exp(n: f64) -> f64 {
        n.exp()
    }
}


enum Trig {
    Sin,
    Cos,
    Tan,
    Asin,
    Acos,
    Atan,
    Sinh,
    Cosh,
    Tanh,
    DegToRad,
    RadToDeg,
}

impl Trig {
    pub fn sin(x: f64) -> f64 {
        x.sin()
    }

    pub fn cos(x: f64) -> f64 {
        x.cos()
    }
    
    pub fn tan(x: f64) -> f64 {
        x.tan()
    }
    
    pub fn asin(x: f64) -> f64 {
        x.asin()
    }
    
    pub fn acos(x: f64) -> f64 {
        x.acos()
    }
    
    pub fn atan(x: f64) -> f64 {
        x.atan()
    }

    pub fn sinh(x: f64) -> f64 {
        x.sinh()
    }
    
    pub fn cosh(x: f64) -> f64 {
        x.cosh()
    }
    
    pub fn tanh(x: f64) -> f64 {
        x.tanh()
    }

    pub fn deg_to_rad(deg: f64) -> f64 {
        deg * std::f64::consts::PI / 180.0
    }
    
    pub fn rad_to_deg(rad: f64) -> f64 {
        rad * 180.0 / std::f64::consts::PI
    }
}


enum Statistics {
    Mean,
    Median,
    Mode,
    Variance,
    StdDev,
    Range,
}

impl Statistics {
    pub fn mean(data: &[f64]) -> f64 {
        let sum: f64 = data.iter().sum();

        sum / data.len() as f64
    }

    pub fn median(data: &mut [f64]) -> f64 {
        data.sort_by(|a, b| a.partial_cmp(b).unwrap());

        let mid = data.len() / 2;
        if data.len() % 2 == 0 {
            (data[mid - 1] + data[mid]) / 2.0
        } else {
            data[mid]
        }
    }

    pub fn variance(data: &[f64]) -> f64 {
        let m = Self::mean(data);

        data.iter().map(|x| (x - m).powf(2.0)).sum::<f64>() / data.len() as f64
    }

    pub fn std_dev(data: &[f64]) -> f64 {
        Self::variance(data).sqrt()
    }

    pub fn range(data: &[f64]) -> f64 {
        let min = data.iter().fold(f64::INFINITY, |a, &b| a.min(b));
        let max = data.iter().fold(f64::NEG_INFINITY, |a, &b| a.max(b));

        max - min
    }
}


enum General {
    Factorial,
    GCD,
    LCM,
    Clamp,
    Min,
    Max,
    Lerp,
}

impl General {
    pub fn fact(n: u64) -> u64 {
        (1..=n).product()
    }

    pub fn gcd(mut a: u64, mut b: u64) -> u64 {
        while b != 0 {
            let tmp = b;
            b = a % b;
            a = tmp;
        }
        a
    }

    pub fn lcm(a: u64, b: u64) -> u64 {
        a * b / Self::gcd(a, b)
    }

    pub fn clamp(n: f64, min: f64, max: f64) -> f64 {
        if n < min {
            min
        } else if n > max {
            max
        } else {
            n
        }
    }

    pub fn min(a: f64, b: f64) -> f64 {
        if a < b {
            a
        } else {
            b
        }
    }

    pub fn max(a: f64, b: f64) -> f64 {
        if a > b {
            a
        } else {
            b
        }
    }

    pub fn lern(start: f64, end: f64, t: f64) -> f64 {
        start + t * (end - start)
    }
}
