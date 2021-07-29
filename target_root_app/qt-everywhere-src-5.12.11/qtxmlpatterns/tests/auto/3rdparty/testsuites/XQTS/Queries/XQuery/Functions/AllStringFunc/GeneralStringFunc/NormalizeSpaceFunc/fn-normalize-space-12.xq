(: Name: fn-normalize-space-12 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing a single tab character. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("	"))