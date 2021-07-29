(: Name: fn-substring-12 :)
(: Description: Evaluation of substring function, where the source string is the empty string :)
(: Use "fn:count" to avoid empty file. :)

fn:count(fn:substring("",0))