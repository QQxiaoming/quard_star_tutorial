(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-15                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Ensure the in-scope prefixes are correct with computed constructors. :)
(:*******************************************************:)
let $i := element e {
element b {()}
}
return (count(in-scope-prefixes(exactly-one($i/b))), count(in-scope-prefixes($i)))
