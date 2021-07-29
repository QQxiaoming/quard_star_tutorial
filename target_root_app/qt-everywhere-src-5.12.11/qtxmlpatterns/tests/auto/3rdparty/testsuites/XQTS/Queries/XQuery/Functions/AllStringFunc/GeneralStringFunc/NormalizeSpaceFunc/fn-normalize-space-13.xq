(: Name: fn-normalize-space-13 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only tab characters. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("			"))