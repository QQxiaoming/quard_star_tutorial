(:*******************************************************:)
(: Test: K2-SeqDeepEqualFunc-23                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Comments inside elements are ignored.        :)
(:*******************************************************:)
declare variable $d1 := <e a="1" b="2"><!-- content -->text</e>;
declare variable $d2 := <e b="2" a="1">text</e>;
deep-equal($d1, $d2),
deep-equal($d2, $d1)