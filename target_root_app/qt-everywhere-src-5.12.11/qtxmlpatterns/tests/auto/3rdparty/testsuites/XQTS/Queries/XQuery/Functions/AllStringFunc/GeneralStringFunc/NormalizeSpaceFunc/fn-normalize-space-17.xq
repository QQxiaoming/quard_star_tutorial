(: Name: fn-normalize-space-17 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only spaces and newline characters.:)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("   


"))