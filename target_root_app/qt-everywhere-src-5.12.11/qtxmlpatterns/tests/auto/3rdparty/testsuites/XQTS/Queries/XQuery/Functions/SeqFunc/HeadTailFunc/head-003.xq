xquery version "1.1";
(:*******************************************************:)
(: Test: head-003                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: head() of a node sequence                  :)
(:*******************************************************:)
let $a := <a><b/><c/><d/></a>
return head($a/*)