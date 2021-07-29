xquery version "1.1";
(:*******************************************************:)
(: Test: tail-005                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: tail() of a node sequence                    :)
(:*******************************************************:)

let $a := <a><b/><c/><d/></a>
return tail($a/*)