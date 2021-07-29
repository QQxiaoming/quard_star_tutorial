xquery version "1.1";
(:*******************************************************:)
(: Test: head-005                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: head() of an empty sequence                  :)
(:*******************************************************:)

count(
  let $a := <a><b/><c/><d/></a>
  return head($a/z)
)