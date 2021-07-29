(:*******************************************************:)
(:Test: adjust-dateTime-to-timezone-17                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The string value of "adjust-dateTime-to-timezone" :)
(: function as part of a boolean (and) expression and the fn:false function. :)
(:*******************************************************:)

fn:string(fn:adjust-dateTime-to-timezone(xs:dateTime("2002-03-07T10:00:00-04:00"),())) and fn:false()