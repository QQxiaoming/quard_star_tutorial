(: Name: fn-not-19 :)
(: Description: Evaluation of an "fn:not" function as arguments to an "fn:concat" function. :)

fn:concat(xs:string(fn:not("true")),xs:string(fn:not("true")))