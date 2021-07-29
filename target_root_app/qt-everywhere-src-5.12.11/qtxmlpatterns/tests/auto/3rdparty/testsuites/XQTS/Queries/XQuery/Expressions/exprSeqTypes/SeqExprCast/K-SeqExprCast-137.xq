(:*******************************************************:)
(: Test: K-SeqExprCast-137                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:base64Binary(xs:hexBinary("ffaabbddcceeff0134f001d8ca9bc77899c83e6f7d"))) eq "/6q73czu/wE08AHYypvHeJnIPm99"`. :)
(:*******************************************************:)
xs:string(xs:base64Binary(xs:hexBinary("ffaabbddcceeff0134f001d8ca9bc77899c83e6f7d")))
		eq "/6q73czu/wE08AHYypvHeJnIPm99"