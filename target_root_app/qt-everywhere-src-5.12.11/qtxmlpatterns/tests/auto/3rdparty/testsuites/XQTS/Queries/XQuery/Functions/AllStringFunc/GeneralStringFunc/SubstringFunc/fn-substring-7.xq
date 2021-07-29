(: Name: fn-substring-7 :)
(: Description: Evaluation of substring function as per example 7 (for this function) :)
(: from the F&O specs.  Use "fn:count" to avoid empty file. :)

fn:count(fn:substring("12345", 0 div 0E0, 3))