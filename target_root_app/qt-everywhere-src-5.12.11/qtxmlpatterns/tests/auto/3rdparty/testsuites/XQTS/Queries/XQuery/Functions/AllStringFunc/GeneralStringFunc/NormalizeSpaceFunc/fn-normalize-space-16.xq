(: Name: fn-normalize-space-16 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only spaces and tab characters.:)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("			   "))