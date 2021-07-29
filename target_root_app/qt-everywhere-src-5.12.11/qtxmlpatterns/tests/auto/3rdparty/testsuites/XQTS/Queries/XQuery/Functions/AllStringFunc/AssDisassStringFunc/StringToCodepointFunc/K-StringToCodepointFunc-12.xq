(:*******************************************************:)
(: Test: K-StringToCodepointFunc-12                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Combine fn:deep-equal and string-to-codepoints(). :)
(:*******************************************************:)
deep-equal(string-to-codepoints("Thérèse"),
						  (84, 104, 233, 114, 232, 115, 101))