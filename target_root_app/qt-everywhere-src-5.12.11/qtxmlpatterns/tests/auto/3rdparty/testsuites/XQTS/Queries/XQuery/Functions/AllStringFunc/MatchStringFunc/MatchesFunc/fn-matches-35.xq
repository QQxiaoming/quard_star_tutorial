(: Name: fn-matches-35 :)
(: Description: Test for bug fix of 5348 in Errata for F&O. Expect FORX0002 err  because \99 is an invalide reference as 99th subexpression does not exist :)
fn:matches('aA', '(a)\99')
