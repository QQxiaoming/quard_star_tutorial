xquery version "1.1";
(:*******************************************************:)
(: Test: head-006                                        :)
(: Written by: Michael Kay                               :)
(: Purpose: head() preserves identity                    :)
(:*******************************************************:)

  let $a := <a><b/><c/><d/></a>
  return head($a/b) is $a/b
