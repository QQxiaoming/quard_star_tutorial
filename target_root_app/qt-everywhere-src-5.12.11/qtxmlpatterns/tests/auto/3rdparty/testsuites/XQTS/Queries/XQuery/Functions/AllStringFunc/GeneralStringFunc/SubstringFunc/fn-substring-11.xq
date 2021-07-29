(: Name: fn-substring-11 :)
(: Description: Evaluation of substring function as per example 11 (for this function) :)
(: from the F&O specs. Use "fn:count" to avoid empty file. :)

fn:count(fn:substring("12345", -1 div 0E0, 1 div 0E0))