(: Name: fn-normalize-space-9 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only spaces. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("   "))