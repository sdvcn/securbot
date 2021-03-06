const puppeteer = require('puppeteer');
const datediff = require('date-diff');


// Genrate list total time spent per user
module.exports.getWorklog = async function (settings)
{
    // Opening chromium
    const browser = await puppeteer.launch({
        args: [ '--no-sandbox', '--disable-setuid-sandbox' ] });

    try {
        const page = await browser.newPage();

        console.log('Chromium started');

        console.log('Pulling worklog');

        // Load Page
        console.log('Loading page...');
        await page.goto(settings.url);
        console.log('Page loaded');

        // Log into Jira
        console.log('Filling login ...');
        await page.type('#login-form-username', settings.user);
        await page.type('#login-form-password', settings.passwd);
        await page.click('#login');

        console.log('Submitting login');
        try {
            await page.waitForNavigation({
                timeout: 10000,
                waitUntil: 'networkidle2'
            });
        } catch( e ) {};

        console.log('Navigating to Worklog');
        await page.goto(settings.worklogUrl, {
            timeout: 60000,
            waitUntil: 'networkidle0'
        });

        // Fetching entries in page
        console.log('Parsing entries');
        var entriesList = await page.$$('.sc-jDwBTQ.kSZgUu');
        entriesList.pop(); // removing total row

        var record = [];
        entriesList.forEach((line)=>{
            var nameP = line.$eval(
                '.sc-gPEVay.gmMOGg>span>span>span',
                element => element.innerText.trim());
            var timeP = line.$$eval(
                '.cell_component>.cell_wrapper_component',
                elementList => parseFloat(elementList[0].innerText.replace('h','').trim()));

            record.push(new Promise( async(resolve, reject) => {
                try{resolve({name: await nameP, time: await timeP})}
                catch(e){reject(e)};
            }));
        });

        record = await Promise.all(record);

        // Computing average time per 7 days according to start date
        console.log('Computing averages');
        var projectDays = new datediff(new Date(), new Date(settings.startDate)).days() - 124;
        record.forEach((entry) =>
        {
            entry.mean = Math.round( 100 * entry.time * 7.0 / projectDays)/100;
        });
    } catch (e) {
        browser.close();
        throw e;
    }

    browser.close();

    return record;
}


// Adds the means in the browser context
module.exports.addMeans= function (records){
    let gadget = document.getElementById("iframe-gadget")
    let lines = gadget.contentDocument.getElementsByClassName('sc-jDwBTQ kSZgUu');

    for(let container of lines) {
        var name = '';
        var nameContainer = null;
        try{
            nameContainer = container.querySelector('.sc-gPEVay.gmMOGg>span>span>span');
            name = nameContainer.innerText.trim();
        } catch(e){};
        if (name == '' || container == null) return;

        records.find((member)=>{
            if (name ==  member.name)
            {
                // Change the field with total
                /*container.querySelectorAll('.total_cell>div>div')[0].innerHTML =
                    `Avg(${member.mean}h) Total`;

                return true;*/

                // Put hour average under name
                nameContainer.innerHTML =
                `${name}<br>
                 Avg. <b>${member.mean}h</b>/sem.`;

                 return true;
            }

            return false;
        })
    }
}
