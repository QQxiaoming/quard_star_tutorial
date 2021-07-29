(:*******************************************************:)
(: Test: K-SeqExprCast-135                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:base64Binary(xs:hexBinary("F43D1234ce8f"))) eq "9D0SNM6P"`. :)
(:*******************************************************:)
xs:string(xs:base64Binary(xs:hexBinary("F43D1234ce8f"))) eq "9D0SNM6P"