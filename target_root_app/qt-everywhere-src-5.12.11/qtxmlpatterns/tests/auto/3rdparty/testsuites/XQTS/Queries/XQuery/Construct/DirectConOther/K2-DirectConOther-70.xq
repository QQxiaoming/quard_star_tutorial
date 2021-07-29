(:*******************************************************:)
(: Test: K2-DirectConOther-70                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that EOLs are normalized in text nodes, but not when specified with character references. :)
(:*******************************************************:)
string-to-codepoints(<a> 
|&#xD; &#xD;&#xA;</a>)