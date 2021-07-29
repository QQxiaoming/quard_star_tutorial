(:*******************************************************:)
(: Test: K2-QuantExprWithout-9                           :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Combine some with fn:deep-equal().           :)
(:*******************************************************:)
let $firstSeq := (<a/>, <b/>, <e><c/></e>)
let $secondSeq := (<a attr=""/>, <b>text</b>, <e><c/></e>)
return some $i in $firstSeq
satisfies $secondSeq[deep-equal(.,$i)]