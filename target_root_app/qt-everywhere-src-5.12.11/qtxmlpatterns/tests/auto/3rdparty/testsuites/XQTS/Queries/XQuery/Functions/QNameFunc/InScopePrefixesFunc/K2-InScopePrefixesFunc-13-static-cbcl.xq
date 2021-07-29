(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-13                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Ensure the in-scope prefixes are correct with computed constructors when combined with a default element namespace declaration. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/";
let $i := element e {
element b {()}
}
return (count(in-scope-prefixes(exactly-one($i/b))), count(in-scope-prefixes($i)))
