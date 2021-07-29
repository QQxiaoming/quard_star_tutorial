(: Name: fn-tokenize-7 :)
(: Description: Evaluation of tokenize function with $input set to empty sequence :)
(: Uses fn:count to avoid empty file. :)

fn:count(fn:tokenize((), "\s+"))