(:*******************************************************:)
(:Test: adjust-date-to-timezone-13                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The string value of "adjust-date-to-timezone" :)
(: function as an argument to the "fn:not" function.     :)
(:*******************************************************:)

fn:not(fn:string(fn:adjust-date-to-timezone(xs:date("2002-03-07-04:00"),())))