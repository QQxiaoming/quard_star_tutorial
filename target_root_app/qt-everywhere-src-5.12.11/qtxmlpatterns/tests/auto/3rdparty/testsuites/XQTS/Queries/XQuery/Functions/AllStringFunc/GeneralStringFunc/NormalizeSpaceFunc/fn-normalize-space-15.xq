(: Name: fn-normalize-space-15 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only multiple newline characters. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("


"))