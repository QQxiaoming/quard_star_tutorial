(: Name: fn-normalize-space-10 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only a single space. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space(" "))