use std::sync::{Mutex,Arc};
use std::thread;

extern crate rand;
use rand::Rng;

fn main() {
	let start_balance = 1000;
	let num_transactions = 1000;
	let num_threads = 10;
	let num_accounts = 1000;
	let max_amount = 20;
	let mut threads = vec![];
	let mut accounts = vec![];

	for _ in 0 .. num_accounts {
		accounts.push(Arc::new(Mutex::new(start_balance)));
	}

	let accounts_protection = Arc::new(Mutex::new(accounts));

	for _ in 0 .. num_threads {
		let a = Arc::clone(&accounts_protection);
		let h = thread::spawn(move || {
			let mut rng = rand::thread_rng();

			for _ in 0 .. num_transactions / num_threads {
				let i0 : usize = rng.gen();
				let i = i0 % num_accounts;

				let j0 : usize = rng.gen();
				let j = j0 % num_accounts;

				let amount0 : i32 = rng.gen();
				let mut amount = amount0 % max_amount;
				if amount < 0 {
					amount = -amount;
				}
				if i != j {
					let array = a.lock().unwrap();
					let mut from = array[i].lock().unwrap();
					let mut to = array[j].lock().unwrap();
					if *from >= amount {
						*from -= amount;
						*to += amount;
					}
				}
			}
		});
		threads.push(h);
	}

	for h in threads {
		h.join().unwrap();
	}

	let mut sum = 0;
	let mut all_at_start_balance = true;
	let array = accounts_protection.lock().unwrap();
	for i in 0 .. num_accounts {
		let x = array[i].lock().unwrap();
		if *x != start_balance {
			all_at_start_balance = false;
		}
		sum += *x;
	}

	if all_at_start_balance {
		println!("YOU PROBABLY DID NOT MODIFY ANY ACCOUNT");
	} else if sum == (num_accounts as i32) * start_balance {
		println!("PASS");
	} else {
		println!("FAIL");
	}
}
