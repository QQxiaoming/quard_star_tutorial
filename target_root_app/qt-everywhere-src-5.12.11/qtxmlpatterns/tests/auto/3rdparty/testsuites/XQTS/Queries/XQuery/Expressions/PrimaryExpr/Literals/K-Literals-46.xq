(:*******************************************************:)
(: Test: K-Literals-46                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:20+01:00                       :)
(: Purpose: Test containing all predefined character references and one hexa and decimal character reference. :)
(:*******************************************************:)
"&lt; &gt; &amp; &quot; &apos; &#x48; &#48;" eq
		"< > &amp; "" ' &#x48; &#48;"
	