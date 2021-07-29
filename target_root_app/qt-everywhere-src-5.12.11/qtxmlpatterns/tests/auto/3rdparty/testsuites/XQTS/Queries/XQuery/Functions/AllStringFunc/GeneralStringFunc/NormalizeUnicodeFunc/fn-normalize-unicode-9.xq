(: Name: fn-normalize-unicode-9 :)
(: Written by: Oliver Hallam :)
(: Date: 2009-12-08 :)
(: Description: Evaluation of fn:normalize-unicode with noncharacters in argument :)

string-to-codepoints(fn:normalize-unicode('e&#x302;&#xfdd0;e&#x302;&#xfdd0;e&#x302;'))
