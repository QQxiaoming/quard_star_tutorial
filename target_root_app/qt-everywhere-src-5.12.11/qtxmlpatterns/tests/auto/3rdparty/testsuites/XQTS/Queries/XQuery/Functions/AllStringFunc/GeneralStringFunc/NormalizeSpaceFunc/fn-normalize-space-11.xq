(: Name: fn-normalize-space-11 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing the zero length string. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space(""))