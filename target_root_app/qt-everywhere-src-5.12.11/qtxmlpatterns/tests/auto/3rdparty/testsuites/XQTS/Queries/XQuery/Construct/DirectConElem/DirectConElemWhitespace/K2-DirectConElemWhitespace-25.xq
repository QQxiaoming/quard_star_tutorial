(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-25                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the string value of mixed content(#3). :)
(:*******************************************************:)
string(<elem><![CDATA[cdata&<>'"< ]]>asda <?target content?>asdad</elem>)
    eq "cdata&amp;<>'""&lt;&#x20;asda asdad"