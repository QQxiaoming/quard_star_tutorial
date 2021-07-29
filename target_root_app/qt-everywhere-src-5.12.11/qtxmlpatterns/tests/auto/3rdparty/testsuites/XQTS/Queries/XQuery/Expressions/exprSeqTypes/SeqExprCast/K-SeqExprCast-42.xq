(:*******************************************************:)
(: Test: K-SeqExprCast-42                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `(xs:QName("fn:local-name") cast as xs:string) eq "fn:local-name"`. :)
(:*******************************************************:)
(xs:QName("fn:local-name") cast as xs:string) eq "fn:local-name"