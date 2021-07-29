(: Name: fn-not-20 :)
(: Description: Evaluation of an "fn:not" function as arguments to an "fn:contains" function. :)

fn:contains(xs:string(fn:not("true")),xs:string(fn:not("true")))