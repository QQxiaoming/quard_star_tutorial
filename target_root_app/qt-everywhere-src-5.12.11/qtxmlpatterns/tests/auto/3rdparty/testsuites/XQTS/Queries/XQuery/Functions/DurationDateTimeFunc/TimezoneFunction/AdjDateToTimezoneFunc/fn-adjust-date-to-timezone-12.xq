(:*******************************************************:)
(:Test: adjust-date-to-timezone-12                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates the string value The "adjust-date-to-timezone" :)
(: function as an argument to a boolean function.        :)
(:*******************************************************:)

fn:boolean(fn:string(fn:adjust-date-to-timezone(xs:date("2002-03-07-04:00"),())))