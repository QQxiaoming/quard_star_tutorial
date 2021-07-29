(:*******************************************************:)
(: Test: K2-NameTest-14                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A nametest being "child::*".                 :)
(:*******************************************************:)
declare variable $var := <elem>text<a/><!-- a comment --><b/><?target data?><c/><![CDATA[more text]]></elem>;
$var/child::*