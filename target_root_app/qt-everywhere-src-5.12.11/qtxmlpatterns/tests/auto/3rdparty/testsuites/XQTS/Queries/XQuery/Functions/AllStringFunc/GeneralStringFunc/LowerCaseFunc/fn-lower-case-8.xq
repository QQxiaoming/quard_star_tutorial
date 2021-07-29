(: Name: fn-lower-case-8 :)
(: Description: Evaluation of lower-case function that uses the empty string as part of argument:)
(: Uses "fn:count" to avoid the empty file  :)

fn:count(fn:lower-case(""))