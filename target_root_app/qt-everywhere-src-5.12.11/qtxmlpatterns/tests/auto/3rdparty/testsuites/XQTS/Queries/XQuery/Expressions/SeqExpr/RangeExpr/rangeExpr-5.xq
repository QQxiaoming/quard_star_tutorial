(: Name: rangeExpr-5 :)
(: Description: Evaluation of a range expression that uses the empty sequence function.:)
(: Uses the count function to avoid empty file. :)

fn:count((1, 2 to ()))

