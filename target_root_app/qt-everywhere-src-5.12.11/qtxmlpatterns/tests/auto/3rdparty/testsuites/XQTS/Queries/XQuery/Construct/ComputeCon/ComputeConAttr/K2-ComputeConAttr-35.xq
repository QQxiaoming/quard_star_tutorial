(:*******************************************************:)
(: Test: K2-ComputeConAttr-35                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Empty CDATA sections generate no text nodes. :)
(:*******************************************************:)
<elem><![CDATA[]]>{attribute name {"content"}}<alem/>
</elem>