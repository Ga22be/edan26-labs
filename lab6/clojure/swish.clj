(def start-balance 1000)
(def num-accounts 6)
;; Max 6000, min 6
(def num-transactions 1000000)
;; 10 => 5s
;; 1000000 => 25s
(def num-threads 8)
(def extra-processing 1000)	
(def max-amount	100)

(defrecord account [balance])

(def accounts (vec (for [i (range num-accounts)] (ref (->account start-balance)))))

(defn do-extra-processing [n]
	(if (>= n 1)
		(recur (- n 1))))
		
(defn swish [from to amount]
        (dosync
	        (do-extra-processing extra-processing)
	        (ref-set (accounts from) (update @(accounts from) :balance - amount))
	        (ref-set (accounts to) (update @(accounts to) :balance + amount)))
        )
(defn work [t]
	(if (>= t 1)
		(do
			(swish (rand-int num-accounts) (rand-int num-accounts) (rand-int max-amount))
			(recur (- t 1)))))

(defn read-balance [a] (:balance a))

(defn sum [hd] (if (empty? hd) 0 (+ (read-balance (first hd)) (sum (rest hd)))))

(defn check [] (= (* num-accounts start-balance) (sum (map deref accounts))))

(defn make-transactions [] (work (/ num-transactions num-threads)))

(defn main []

	(println "swish with clojure software transactional memory")
	(println "accounts:      " num-accounts)
	(println "transactions:  " num-transactions)
	(println "threads:       " num-threads)

	(let [threads (repeatedly num-threads #(Thread. make-transactions))]
		(run! #(.start %) threads)
		(run! #(.join %) threads))

	(if (check) (println "PASS") (println "FAIL")))


(main)
(println (map deref accounts))
