(:*******************************************************:)
(: Test: K2-LetExprWithout-15                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: let-declarations doesn't cause URI promotion conversion. :)
(:*******************************************************:)
let $i as xs:string := xs:anyURI("http://www.example.com/")
return $i