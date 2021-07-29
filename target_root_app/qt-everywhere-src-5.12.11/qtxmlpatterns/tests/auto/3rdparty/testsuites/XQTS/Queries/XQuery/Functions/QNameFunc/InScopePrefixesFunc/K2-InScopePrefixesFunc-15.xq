(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-15                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the in-scope prefixes are correct with computed constructors. :)
(:*******************************************************:)
let $i := element e {
element b {()}
}
return (count(in-scope-prefixes($i/b)), count(in-scope-prefixes($i)))