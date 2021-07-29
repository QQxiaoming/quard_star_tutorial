(: Name: fn-normalize-unicode-10 :)
(: Written by: Oliver Hallam :)
(: Date: 2009-12-08 :)
(: Description: Evaluation of fn:normalize-unicode with noncharacters in argument :)

string-to-codepoints(fn:normalize-unicode('e&#xfdd0;&#x302;'))
