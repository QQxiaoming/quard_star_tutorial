(: Name: fn-substring-8 :)
(: Description: Evaluation of substring function as per example 8 (for this function) :)
(: from the F&O specs. Use "fn:count" to avoid empty file. :)

fn:count(fn:substring("12345", 1, 0 div 0E0))