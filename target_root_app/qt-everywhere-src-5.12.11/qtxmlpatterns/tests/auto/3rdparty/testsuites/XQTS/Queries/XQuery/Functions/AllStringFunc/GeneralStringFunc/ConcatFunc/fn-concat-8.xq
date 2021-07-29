(: Name: fn-concat-8 :)
(: Description: Evaluation of concat function that uses the empty string as part of argument:)
(: Uses "fn:count" to avoid the empty file  :)

fn:count(fn:concat("",""))