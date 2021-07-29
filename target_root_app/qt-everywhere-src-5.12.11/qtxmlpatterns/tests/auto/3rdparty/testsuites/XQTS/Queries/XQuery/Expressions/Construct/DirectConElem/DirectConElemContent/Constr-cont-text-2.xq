(: Name: Constr-cont-text-2 :)
(: Written by: Andreas Behm :)
(: Description: count text nodes for direct element content and CDATA section :)

count((<elem>text<![CDATA[cdata]]></elem>)/text())
