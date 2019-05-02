extern crate zipf;
#[macro_use(crate_version, crate_authors)]
extern crate clap;

use clap::{App, Arg};

use zipf::ZipfDistribution;

use rand::thread_rng;
use rand::distributions::Distribution;

fn main() {
	let cmd_args = App::new("power-law")
		.version(crate_version!())
		.author(crate_authors!())
		.about("Generate a sample according to power law distribution")
		.arg(
			Arg::with_name("alpha")
				.short("a")
				.long("alpha")
				.takes_value(true)
				.required(true)
				.help("Alpha parameter of the power law distribution")
		)
		.arg(
			Arg::with_name("max-val")
				.short("m")
				.long("maxval")
				.takes_value(true)
				.required(true)
				.help("Maximum value of the sample")
		)
		.arg(
			Arg::with_name("num-points")
				.short("n")
				.long("numpoints")
				.value_name("NUMPOINTS")
				.takes_value(true)
				.required(true)
				.help("Total number of points in the sample")
		)
		.get_matches();
		
	let alpha = cmd_args.value_of("alpha").unwrap().parse::<f64>().unwrap();
	let xmax = cmd_args.value_of("max-val").unwrap().parse::<usize>().unwrap();
	let num = cmd_args.value_of("num-points").unwrap().parse::<usize>().unwrap();

	let dist: ZipfDistribution = ZipfDistribution::new(xmax, alpha).unwrap();
	for _ in 0..num {
		println!("{}", dist.sample(&mut thread_rng()));
    	}
}
