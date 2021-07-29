(: Name: fn-normalize-space-14 :)
(: Description: Evaluation of fn-normalize-space function with argument string containing only a single newline character. :)
(: Use fn:count to avoid empty file  :)

fn:count(fn:normalize-space("
"))