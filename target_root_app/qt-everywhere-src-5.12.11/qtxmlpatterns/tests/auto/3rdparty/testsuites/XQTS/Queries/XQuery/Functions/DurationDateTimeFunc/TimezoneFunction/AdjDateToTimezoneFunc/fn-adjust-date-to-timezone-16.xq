(:*******************************************************:)
(:Test: adjust-date-to-timezone-16                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The string value of "adjust-date-to-timezone" :)
(: function as part of a boolean (and) expression and the fn:true function. :)
(:*******************************************************:)

fn:string(fn:adjust-date-to-timezone(xs:date("2002-03-07-04:00"),())) and fn:true()