(: Name: fn-normalize-unicode-8 :)
(: Written by: Oliver Hallam :)
(: Date: 2009-12-08 :)
(: Description: Evaluation of fn:normalize-unicode with noncharacter argument :)

string-to-codepoints(normalize-unicode('&#xfdd0;'))
