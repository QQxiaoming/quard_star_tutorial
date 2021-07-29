(:********************************************************:)
(: Test: boundary-space-007.xq                            :)
(: Written By: Pulkita Tyagi                              :)
(: Date: Fri Jun 17 06:27:51 2005                         :)
(: Purpose: Demonstrates stripping/preserving of boundary :)
(:          spaces by element constructors during processing of the query :)
(:************************************************************************:)

declare boundary-space strip;
<a>&#x20;{"abc"}{' '}</a>
