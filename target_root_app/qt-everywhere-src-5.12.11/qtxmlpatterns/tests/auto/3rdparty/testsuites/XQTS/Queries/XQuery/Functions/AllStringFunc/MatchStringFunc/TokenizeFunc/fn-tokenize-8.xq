(: Name: fn-tokenize-8 :)
(: Description: Evaluation of tokenize function with $input set to zero length string. :)
(: Uses fn:count to avoid empty file. :)

fn:count(fn:tokenize("", "\s+"))