// 
//  We're doing this simply because we can't just "ask"
//   how many registers are available on the CPU.
// 
typedef struct TargetConfig {
	int available_registers;      // Total available general-purpose registers.
	char** register_names;        // The names (e.g., "rax", "rbx")
	int* caller_saved_indices;    // Which registers are safe to remove?
} TargetConfig;