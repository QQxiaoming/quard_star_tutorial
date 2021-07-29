(: Name: fn-matches-37 :)
(: Description: Test for bug fix of 5348 in Errata for F&O. Expect FORX0002 err  because \11 reference is made before the closing right parenthesis of 11th reference:)
fn:matches('abcdefghijk', '(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k\11)')
