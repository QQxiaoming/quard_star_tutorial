(: Name: fn-tokenize-5 :)
(: Description: Evaluation of tokenize function with pattern set to "\s*<br>\s*" and flag set to "i" as :)
(: per example 4 for this functions from the Func and Ops specs. :)

fn:tokenize("Some unparsed <br> HTML <BR> text", "\s*<br>\s*", "i")