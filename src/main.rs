extern crate zipf;
use zipf::ZipfDistribution;
use std::env;
use rand::thread_rng;
use rand::distributions::Distribution;

fn main() {
	let args: Vec<String> = env::args().collect();
	let alpha: f64 = args[1].parse().unwrap();
	let xmax: usize = 2 << 20;
	let dist: ZipfDistribution = ZipfDistribution::new(xmax, alpha).unwrap();
	for _ in 0..xmax {
		println!("{}", dist.sample(&mut thread_rng()));
    	}
}
