(:*******************************************************:)
(: Test: K2-LetExprWithout-2                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: fn:deep-equal combined with a for/let expression(#2). :)
(:*******************************************************:)
deep-equal((<b/>, <b/>, <b/>, <b/>),
            (for $v1 in (1, 2, 3, 4)
             let $v2 := <b/>
             return ($v2)))