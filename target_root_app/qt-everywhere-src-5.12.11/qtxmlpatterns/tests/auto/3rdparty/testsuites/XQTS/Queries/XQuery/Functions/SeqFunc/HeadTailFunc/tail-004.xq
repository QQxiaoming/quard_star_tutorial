xquery version "1.1";
(:*******************************************************:)
(: Test: tail-004                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: tail() of an empty sequence                  :)
(:*******************************************************:)

let $a := <a><b/><c/><d/></a>
return count(tail($a/z))