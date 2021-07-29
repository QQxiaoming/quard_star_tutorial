(:*******************************************************:)
(: Test: K2-SeqDeepEqualFunc-16                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure processing instructions are ignored if children of a document node. :)
(:*******************************************************:)
declare variable $d1 := document { <?target data?>,
text{"some text"}};
declare variable $d2 := document {text{"some text"}};
deep-equal($d1, $d2)