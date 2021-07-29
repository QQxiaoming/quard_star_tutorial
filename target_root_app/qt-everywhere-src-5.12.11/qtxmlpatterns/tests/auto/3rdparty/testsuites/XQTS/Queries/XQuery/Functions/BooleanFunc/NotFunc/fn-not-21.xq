(: Name: fn-not-21 :)
(: Description: Evaluation of an "fn:not" function as an argument to an "fn:string-length" function. :)

fn:string-length(xs:string(fn:not("true")))