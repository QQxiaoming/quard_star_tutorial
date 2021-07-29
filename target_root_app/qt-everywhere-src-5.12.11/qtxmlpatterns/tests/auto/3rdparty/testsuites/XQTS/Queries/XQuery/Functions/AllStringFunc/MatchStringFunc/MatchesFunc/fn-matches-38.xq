(: Name: fn-matches-38                                                        :)
(: Description: Test for bug fix of 5348 in Errata for F&O.                   :)
(: Expect FORX0002 err  because \10 reference is made before the closing      :)
(: right parenthesis of 10th reference                                        :)

fn:matches('abcdefghijj', '(a)(b)(c)(d)(e)(f)(g)(h)(i)(j\10)')
