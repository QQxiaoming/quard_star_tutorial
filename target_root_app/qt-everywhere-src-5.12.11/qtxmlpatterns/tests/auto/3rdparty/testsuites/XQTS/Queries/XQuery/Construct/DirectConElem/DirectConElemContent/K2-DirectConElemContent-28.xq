(:*******************************************************:)
(: Test: K2-DirectConElemContent-28                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure CDATA doesn't receive special attention. :)
(:*******************************************************:)
string(<elem><![CDATA[con<<< ]] >""'*"*">>tent]]&#00;&#x12;&amp;&quot;&notrecognized;&apos]]></elem>)
eq "con&lt;&lt;&lt; ]] &gt;""""'*""*""&gt;&gt;tent]]&amp;#00;&amp;#x12;&amp;amp;&amp;quot;&amp;notrecognized;&amp;apos"